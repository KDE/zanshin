Feature: Context rename
  As someone collecting tasks
  I can rename a context
  In order to refine my tasks organization

  Scenario: Renamed context appear in the list
    Given I display the available pages
    When I rename the page named "Errands" under "Contexts" to "Chores"
    And I list the items
    Then the list is:
       | display                                       | icon              |
       | Inbox                                         | mail-folder-inbox |
       | Workday                                       | go-jump-today     |
       | Projects                                      | folder            |
       | Projects / Calendar1                          | folder            |
       | Projects / Calendar1 / Prepare talk about TDD | view-pim-tasks    |
       | Projects / Calendar1 / Read List              | view-pim-tasks    |
       | Projects / Calendar2                          | folder            |
       | Projects / Calendar2 / Backlog                | view-pim-tasks    |
       | Contexts                                      | folder            |
       | Contexts / Chores                             | view-pim-notes    |
       | Contexts / Online                             | view-pim-notes    |
